tmp<fv::convectionScheme<scalar>> mvConvection(nullptr);

if (Y.size())
{
    mvConvection = tmp<fv::convectionScheme<scalar>>
    (
        fv::convectionScheme<scalar>::New
        (
            mesh,
            fields,
            phi,
            mesh.divScheme("div(phi,Yi_h)")
        )
    );
}

{
    combustion->correct();

    tmp<volScalarField> Yt(nullptr);

    if (Y.size())
    {
        Yt = tmp<volScalarField>
        (
            new volScalarField
            (
                IOobject("Yt", runTime.timeName(), mesh),
                mesh,
                dimensionedScalar(dimless, 0)
            )
        );
    }

    forAll(Y, i)
    {
        if (i != inertIndex && composition.active(i))
        {
            volScalarField& Yi = Y[i];

            fvScalarMatrix YEqn
            (
                fvm::ddt(rho, Yi)
              + mvConvection->fvmDiv(phi, Yi)
              + thermophysicalTransport->divj(Yi)
              ==
                clouds.SYi(i, Yi)
              + fvOptions(rho, Yi)
              + combustion->R(Yi)
              + surfaceFilm.Srho(i)
            );

            YEqn.relax();

            fvOptions.constrain(YEqn);

            YEqn.solve("Yi");

            fvOptions.correct(Yi);

            Yi.max(0.0);
            Yt.ref() += Yi;
        }
    }

    if (Y.size())
    {
        Y[inertIndex] = scalar(1) - Yt;
        Y[inertIndex].max(0.0);
    }
}
